require 'test_helper'

class ImageCategoriesControllerTest < ActionController::TestCase
  setup do
    @image_category = image_categories(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:image_categories)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create image_category" do
    assert_difference('ImageCategory.count') do
      post :create, image_category: {  }
    end

    assert_redirected_to image_category_path(assigns(:image_category))
  end

  test "should show image_category" do
    get :show, id: @image_category
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @image_category
    assert_response :success
  end

  test "should update image_category" do
    put :update, id: @image_category, image_category: {  }
    assert_redirected_to image_category_path(assigns(:image_category))
  end

  test "should destroy image_category" do
    assert_difference('ImageCategory.count', -1) do
      delete :destroy, id: @image_category
    end

    assert_redirected_to image_categories_path
  end
end
